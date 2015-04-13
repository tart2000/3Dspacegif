
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
    <nav class="pagination">

    <?php if($gifpage->pagination()->hasNextPage()): ?>
    <a class="next" href="<?php echo $articles->pagination()->nextPageURL() ?>">&lsaquo; newer posts</a>
    <?php endif ?>

    <?php if($gifpage->pagination()->hasPrevPage()): ?>
    <a class="prev" href="<?php echo $articles->pagination()->prevPageURL() ?>">older posts &rsaquo;</a>
    <?php endif ?>

    </nav>
  <?php endif ?>
</div>

