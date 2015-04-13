
<div class="row">
  <?php $gifpage = page('gifs')->images()->flip()->paginate(12) ?>

  <?php foreach ($gifpage as $image) : ?>
    <div class="col-md-3 col-sm-4 col-xs-6">
      <a href="<?php echo $image->url() ?>">
        <img src="<?php echo $image->url() ?>" alt="" class="img-responsive impb">
      </a>
    </div>
  <?php endforeach ?>

</div>

<div class="row">
  <?php if($gifpage->pagination()->hasPages()): ?>
    <nav>
      <ul class="pager">
        <?php if($gifpage->pagination()->hasNextPage()): ?>
          <li class="previous"><a href="<?php echo $gifpage->pagination()->nextPageURL() ?>"><span aria-hidden="true">&larr;</span> Older</a></li>
        <?php endif ?>
        <?php if($gifpage->pagination()->hasPrevPage()): ?>
          <li class="next"><a href="<?php echo $gifpage->pagination()->prevPageURL() ?>">Newer <span aria-hidden="true">&rarr;</span></a></li>
        <?php endif ?>
      </ul>
    </nav>
  <?php endif ?>
</div>

